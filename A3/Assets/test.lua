rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
--green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
--white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

head = gr.mesh('head', 'name')
head:translate(-2.75,0.0,-10.0)
head:set_material(red)

rootnode:add_child(head)
return rootnode
