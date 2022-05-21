rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)

-------------------------------------------------------------------------------------------
--Geometry for first pendulum
first = gr.mesh('cube', 'first')
first:set_material(red)
first:scale(0.5, 2.5, 0.5)

-------------------------------------------------------------------------------------------
--Geometry for second pendulum
second = gr.mesh('cube', 'second')
second:set_material(blue)
second:scale(0.5,2.5,0.5)
-- After the scale, the top point of the pendulum is at (0.25, 1.25, 0.25)
-- We translate so that the top of the pendulum is at the origin in model coordinates
second:translate(0,-1.25,0) --Question: should we be pass translations down in geometry nodes?? 

-------------------------------------------------------------------------------------------
-- We use intermediate nodes for the relationship between the first pendulum and the second
second_t = gr.node('second-translation')
-- translation for the second pendulum, so that the top of first pendulum is at the bottom of the first
second_t:translate(0, -1, 0)

second_joint_t = gr.node('first-second', {0,0,90}, {0,0,90})
--second_joint_t:rotate('z', 45)
second_t:add_child(second_joint_t)
-------------------------------------------------------------------------------------------
--Geometry for third pendulum
third = gr.mesh('cube', 'third')
third:set_material(green)
third:scale(0.5,3,0.5)

--After the scale, the top point of the pendulum is at (0.25,1.5,0.25).
--Translate so that the top of the pendulum is at origin in model coordinates
third:translate(0,-1.5,0)
-------------------------------------------------------------------------------------------
-- Intermediate nodes for second to third pendulum
third_t = gr.node('third-translation')
--Before rotations, bottom of second pendulum is at (0.25,-2.25,0.25). So we translate down there
third_t:translate(0, -2, 0)

third_joint_t = gr.node('second-third-joint', {0,0,90}, {0,0,90})
third_t:add_child(third_joint_t)

-------------------------------------------------------------------------------------------
--Now form the pendulum tree
rootnode:add_child(first)
first:add_child(second_t)
second_joint_t:add_child(second)
second:add_child(third_t)
third_joint_t:add_child(third)

-- Transform joint nodes. This is how the user will be interacting with the model
first:rotate('z', 30)
second_joint_t:rotate('z',45)
third_joint_t:rotate('z',45)

--Translate the whole model so that it is visible
rootnode:translate(0.0, 0.0, -10.0)

return rootnode
