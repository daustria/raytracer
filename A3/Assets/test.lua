rootnode = gr.node('root')

--red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
--blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('torso', 'blathers-torso')
torso:set_material(white)

head = gr.mesh('head', 'blathers-head')
head:set_material(white)
torso:add_child(head)



rootnode:add_child(torso)
rootnode:translate(0,0,-10)
return rootnode
