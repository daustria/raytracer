
mat = gr.material_texture('capsule.mtl')

scene = gr.node( 'scene' )

capsule = gr.mesh( 'capsule', 'capsule.obj' )
capsule:set_material(mat)
capsule:translate(0,0,-10)
scene:add_child(capsule)

light1 = gr.light({0, 500, -400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
light2 = gr.light({0,200,-200}, {0.8, 0.8, 0.8}, {1,0,0})

gr.render(scene, 'capsule.png', 256, 256,
	  {0, 0, 3}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {light1, light2})
