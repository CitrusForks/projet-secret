import gs
import pymsgbox
from demo_simulation import DemoSimulation
from utils import *


def main():
	pc_screen_windowed = True
	pc_screen_width = 1280
	pc_screen_height = 720

	plus = gs.GetPlus()
	# plus.CreateWorkers()

	# window_mode = pymsgbox.confirm(text='Select your screen mode', title='System Zoetrope', buttons=['Windowed', 'Fullscreen'])
	#
	# if window_mode == 'Windowed':
	# 	pc_screen_windowed = True
	# 	screen_resolutions = ['640x480', '720x568', '800x600', '1280x800']
	# elif window_mode == 'Fullscreen':
	# 	pc_screen_windowed = False
	# 	screen_resolutions = ['640x480', '800x600', '1280x720', '1280x800', '1920x1080']
	# else:
	# 	return False
	#
	# screen_res = pymsgbox.confirm(text='Select your screen resolution', title='System Zoetrope',
	# 							   buttons=screen_resolutions)
	#
	# if screen_res is not None:
	# 	pc_screen_width = int(screen_res.split('x')[0])
	# 	pc_screen_height = int(screen_res.split('x')[1])
	# else:
	# 	return False

	demo_screen_width = 720//2
	demo_screen_height = 568//2
	amiga_screen_ratio = demo_screen_height / demo_screen_width
	overscan_factor = gs.Vector4(16 / demo_screen_width, 4 / demo_screen_height,
								 (demo_screen_width - 16) / demo_screen_width, (demo_screen_height - 28) / demo_screen_height) # in pixels


	if pc_screen_windowed:
		pc_screen_width = int(pc_screen_height * (demo_screen_width / demo_screen_height))

	# mount the system file driver
	# gs.GetFilesystem().Mount(gs.StdFileDriver("pkg.core"), "@core")
	gs.MountFileDriver(gs.StdFileDriver())
	gs.LoadPlugins(gs.get_default_plugins_path())

	# create the renderer and render system
	if pc_screen_windowed:
		w_mode = gs.Window.Windowed
	else:
		w_mode = gs.Window.Fullscreen
	plus.RenderInit(pc_screen_width, pc_screen_height, 1, w_mode)

	egl = plus.GetRenderer()

	# create the font object
	font = gs.RasterFont("@core/fonts/default.ttf", 48, 512)

	# Init demo simulation
	demo = DemoSimulation(demo_screen_width, demo_screen_height)

	# load a simple 2d shader outputting a single color
	shader = egl.LoadShader("res/shader_2d_single_texture.isl")

	# Create index buffer
	data = gs.BinaryBlob()
	data.WriteShorts([0, 1, 2, 0, 2, 3])

	idx = egl.NewBuffer()
	egl.CreateBuffer(idx, data, gs.GpuBuffer.Index)

	# Create vertex buffer
	# Create vertex buffer
	vtx_layout = gs.VertexLayout()
	vtx_layout.AddAttribute(gs.VertexAttribute.Position, 3, gs.VertexFloat)
	vtx_layout.AddAttribute(gs.VertexAttribute.UV0, 2, gs.VertexUByte, True)  # UVs are sent as normalized 8 bit unsigned integer (range [0;255])

	def custom_uv(u, v):
		return [int(Clamp(u, 0, 1) * 255), int(Clamp(v, 0, 1) * 255)]

	data = gs.BinaryBlob()
	x, y = 1, 1
	data.WriteFloats([-x, -y, 0.5])
	data.WriteUnsignedBytes(custom_uv(overscan_factor.x, overscan_factor.w))
	data.WriteFloats([-x, y, 0.5])
	data.WriteUnsignedBytes(custom_uv(overscan_factor.x, overscan_factor.y))
	data.WriteFloats([x, y, 0.5])
	data.WriteUnsignedBytes(custom_uv(overscan_factor.z, overscan_factor.y))
	data.WriteFloats([x, -y, 0.5])
	data.WriteUnsignedBytes(custom_uv(overscan_factor.z, overscan_factor.w))

	vtx = egl.NewBuffer()
	egl.CreateBuffer(vtx, data, gs.GpuBuffer.Vertex)

	# demo bitmaps
	demo.load_textures()
	demo_screen_tex = egl.NewTexture("demo_screen_texture")

	res = egl.CreateTexture(demo_screen_tex, demo.screen_pic)
	print("CreateTexture() returned ", res)

	# play music
	al = gs.MixerAsync(gs.ALMixer())
	al.Open()
	channel_state = gs.MixerChannelState(0, 1, gs.MixerRepeat)
	al.Stream("res/music_loop.ogg", channel_state)

	mode_switch = "DMODE_SW_UBOB"

	while not plus.IsAppEnded(plus.EndOnDefaultWindowClosed) and not plus.KeyPress(gs.InputDevice.KeyEscape):
		dt = plus.UpdateClock()
		plus.Clear()

		# Demo simulation (re-creation)
		demo.update_dt(dt.to_sec())
		demo.clear_screen()
		demo.draw_pixel_art_logo()
		demo.draw_checkerboard()

		if mode_switch == "DMODE_SW_UBOB":
			if not demo.draw_unlimited_bobs():
				if demo.figure_mode % 2 == 0:
					mode_switch = "DMODE_SW_CLEAR_FROM_TOP"
				else:
					mode_switch = "DMODE_SW_CLEAR_FROM_BOTTOM"
		elif mode_switch == "DMODE_SW_CLEAR_FROM_TOP":
			if demo.clear_playfield(True):
				mode_switch = "DMODE_SW_NEXT_UBOB"
		elif mode_switch == "DMODE_SW_CLEAR_FROM_BOTTOM":
			if demo.clear_playfield(False):
				mode_switch = "DMODE_SW_NEXT_UBOB"
		elif mode_switch == "DMODE_SW_NEXT_UBOB":
			demo.set_next_unlimited_bobs()
			mode_switch = "DMODE_SW_UBOB"

		demo.render_demo_text()

		egl.BlitTexture(demo_screen_tex, gs.BinaryBlobFromByteArray(demo.screen_pic.GetData()), demo_screen_width, demo_screen_height)

		if pc_screen_windowed:

			egl.SetShader(shader)
			egl.SetShaderTexture("u_tex", demo_screen_tex)
			gs.DrawBuffers(egl, 6, idx, vtx, vtx_layout)
			# plus.Quad2D(0, 0, 0, pc_screen_height, pc_screen_width, pc_screen_height,
			# 			pc_screen_width, 0, gs.Color.White, gs.Color.White, gs.Color.White, gs.Color.White,
			# 			demo_screen_tex, overscan_factor.x / demo_screen_width, overscan_factor.y / demo_screen_height,
			# 			(demo_screen_width - overscan_factor.z) / demo_screen_width,
			# 			(demo_screen_height - overscan_factor.w) / demo_screen_height)
		else:
			_x_offset = (pc_screen_width - pc_screen_width * amiga_screen_ratio) * 0.5
			plus.Quad2D(_x_offset, 0, _x_offset, pc_screen_height, _x_offset + (pc_screen_width * amiga_screen_ratio),
						pc_screen_height, _x_offset + (pc_screen_width * amiga_screen_ratio), 0,
						gs.Color.White, gs.Color.White, gs.Color.White, gs.Color.White, demo_screen_tex,
						overscan_factor.x / demo_screen_width, overscan_factor.y / demo_screen_height,
						(demo_screen_width - overscan_factor.z) / demo_screen_width,
						(demo_screen_height - overscan_factor.w) / demo_screen_height)

		plus.Flip()

if __name__ == "__main__":
    main()

