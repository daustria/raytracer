rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)

first = gr.mesh('cube', 'first')
first:set_material(red)
first:scale(0.5, 2.5, 0.5)


--Rotate first pendulum
first:rotate('z', 30)
rootnode:add_child(first)


second = gr.mesh('cube', 'second')
second:set_material(blue)
second:scale(0.5,2.5,0.5)


-- After the scale, the top point of the pendulum is at (0.25, 1.25, 0.25)
-- We translate so that the top of the pendulum is at the origin in model coordinates
second:translate(0,-1.25,0)

-- Rotate pendulum
second:rotate('z', 15)

--Now move it so that the top of the pendulum is at the bottom of the first pendulum
second:translate(0.0, -1, 0)

--Add the second pendulum as a child of the first
first:add_child(second)

--Translate the whole model so that it is visible
rootnode:translate(0.0, 0.0, -10.0)
return rootnode
