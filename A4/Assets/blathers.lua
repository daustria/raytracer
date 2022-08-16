mat = gr.material_texture('blathers.mtl')

scene = gr.node( 'scene' )

blathers = gr.mesh( 'owl', 'blathers.obj' )
blathers:set_material(mat)
--blathers:scale(0.5,0.5,0.5)
--blathers:rotate('y',-40)
--blathers:translate(0,-2,-10)
scene:add_child(blathers)

light1 = gr.light({0, 100, 50}, {1.0, 1.0, 1.0}, {1, 0, 0})
light2 = gr.light({0,-100,-100}, {0.8, 0.8, 0.8}, {1,0,0})
light3 = gr.light({0,20,50}, {0.8, 0.8, 0.8}, {1,0,0})

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene, 'blathers.png', 256, 256,
	  {0, 5, 20}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {light1})
