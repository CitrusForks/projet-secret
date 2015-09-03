import os
import gs
import gs.plus.render as render
import gs.plus.camera as camera
import gs.plus.geometry as geometry
import gs.plus.input as input
import gs.plus.scene as scene
import gs.plus.clock as clock
import gs.plus.audio as audio
import math
from demo_simulation import DemoSimulation


def main():
	demo_screen_width = 384
	demo_screen_height = 280

	if os.path.exists("pkg.core"):
		render.init(1280, 720, "pkg.core")
	else:
		print("Cannot find pkg.core/")
		exit()

	gs.MountFileDriver(gs.StdFileDriver())

	scn = scene.new_scene()

	cam = scene.add_camera(scn, gs.Matrix4.TranslationMatrix(gs.Vector3(0, 0, 0)))
	scene.add_plane(scn)

	scn.Load(os.path.join("res/a500/a500.scn"), gs.SceneLoadContext(render.get_render_system()))
	scn.Load(os.path.join("res/monitor/monitor.scn"), gs.SceneLoadContext(render.get_render_system()))

	scene.update_scene(scn, 0.0)
	tmp_node = scn.GetNode('monitor')
	tmp_node_transform = tmp_node.GetComponentsWithAspect("Transform")[0]
	tmp_pos = tmp_node_transform.GetPosition()
	tmp_pos.z += 0.5
	tmp_node_transform.SetPosition(tmp_pos)

	demo_screen_node = scn.GetNode('monitor_screen')
	c_list = demo_screen_node.GetComponents()
	demo_screen_object = demo_screen_node.GetComponentsWithAspect("Renderable")[0]
	demo_screen_geo = demo_screen_object.GetGeometry()
	demo_screen_mat = demo_screen_geo.GetMaterial(0)
	demo_screen_tex = demo_screen_mat.GetTexture("diffuse_map")

	fps = camera.fps_controller(0, 0.5, -3.5)

	# Init demo simulation
	demo = DemoSimulation(demo_screen_width, demo_screen_height)
	demo.load_textures()

	audio.init()
	demo_audio_stream = audio.get_mixer().Stream(os.path.join("res", "music_loop.ogg"))
	channel_state = audio.get_mixer().GetChannelState(demo_audio_stream)
	channel_state.loop_mode = gs.MixerRepeat
	audio.get_mixer().SetChannelState(demo_audio_stream, channel_state)
	plane_angle = 0.0

	scn.SetCurrentCamera(cam)

	while not input.key_press(gs.InputDevice.KeyEscape):
		dt_sec = clock.update()

		# Demo simulation (re-creation)
		demo.dt = dt_sec
		demo.clear_screen()
		demo.draw_pixel_art_logo()
		demo.draw_checkerboard()
		demo.draw_unlimited_bobs()
		demo.render_demo_text()
		render.get_renderer().BlitTexture(demo_screen_tex, demo.screen_pic)

		fps.update_and_apply_to_node(cam, dt_sec * 0.1)

		scene.update_scene(scn, dt_sec)
		render.text2d(5, 5, "Move around with QSZD, left mouse button to look around")
		render.flip()

	font = None

if __name__ == "__main__":
    main()
