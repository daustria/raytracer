-- A simple scene with five spheres

mat1 = gr.material({1.0, 1.0, 1.0}, {0.5, 0.5, 0.5}, 25)

scene_root = gr.node('root')

cube = gr.mesh('cube', 'cube.obj')
cube:set_material(mat1)
cube:rotate('x',20)
cube:rotate('y',40)
cube:translate(0,0,-10)
scene_root:add_child(cube)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})
light1 = gr.light({0, 200, 0}, {1.0, 1.0, 1.0}, {1, 0, 0})
light2 = gr.light({0,-100,-100}, {0.8, 0.8, 0.8}, {1,0,0})

gr.render(scene_root, 'test.png', 256, 256,
	  {0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {light1, light2})
