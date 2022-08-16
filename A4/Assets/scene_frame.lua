stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

scene = gr.node('scene')

frame = gr.mesh('frame', 'scene_frame.obj')
frame:set_material(stone)
scene:add_child(frame)

-- light
light1 = gr.light({0, 3, -10}, {1.0, 1.0, 1.0}, {1,0,0})

gr.render(scene,
	  'scene.png', 212, 212,
	  {0, 4, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {light1})
