mat = gr.material_texture('isabelle.mtl')

scene = gr.node( 'scene' )

isabelle = gr.mesh( 'dog', 'isabelle.obj' )
isabelle:set_material(mat)
isabelle:translate(0,-7,-15)
scene:add_child(isabelle)

light1 = gr.light({0, 200, -400.0}, {1.0, 1.0, 1.0}, {1, 0, 0})
light2 = gr.light({0,-100,-100}, {0.8, 0.8, 0.8}, {1,0,0})

gr.render(scene, 'isabelle.png', 200, 200,
	  {0, 0, 5}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {light1, light2})
