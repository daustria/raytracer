rootnode = gr.node('root')

--red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
--blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('torso', 'torso')
torso:set_material(white)

head = gr.mesh('head', 'head')
head:set_material(white)

-- Set the head as a joint of the torso
head_joint_t = gr.joint('head-joint', {0,0,0}, {-360,0,360})

head_joint_t:add_child(head)

torso:add_child(head_joint_t)

-- Make the left wing

hi_lwing = gr.mesh('upper-lwing', 'upper-left-wing')
hi_lwing:set_material(green)

lo_lwing = gr.mesh('lower-lwing', 'lower-left-wing')
lo_lwing:set_material(green)

hi_left_joint = gr.joint('upper-left-joint', {-15,0,15}, {-15,0,15})
lo_left_joint = gr.joint('lower-left-joint', {-1,0,1}, {-1,0,1})

-- Connect the left wing nodes
hi_left_joint:add_child(hi_lwing)
hi_lwing:add_child(lo_left_joint)
lo_left_joint:add_child(lo_lwing)
torso:add_child(hi_left_joint)

-- Make the right wing

hi_rwing = gr.mesh('upper-rwing', 'upper-right-wing')
hi_rwing:set_material(green)

lo_rwing = gr.mesh('lower-rwing', 'lower-right-wing')
lo_rwing:set_material(green)

hi_right_joint = gr.joint('upper-right-joint', {-3,0,3}, {-3,0,3})
lo_right_joint = gr.joint('lower-right-joint', {-1,0,1}, {-1,0,1})

-- Connect the right wing nodes

hi_right_joint:add_child(hi_rwing)
hi_rwing:add_child(lo_right_joint)
lo_right_joint:add_child(lo_rwing)

torso:add_child(hi_right_joint)



rootnode:add_child(torso)
rootnode:translate(0,0,-10)
return rootnode
