-- A simple test scene featuring some spherical cows grazing
-- around Stonehenge.  "Assume that cows are spheres..."

stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

-- ##############################################
-- the scene
-- ##############################################

scene = gr.node('scene')
scene:rotate('X', 23)

-- the floor

plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(30, 30, 30)


museum = gr.mesh('museum', 'museum.obj')
--museum:rotate('y',45)
museum:set_material(stone)
museum:scale(2.0,2.0,2.0)
scene:add_child(museum)

light1 = gr.light({0, 100, 350}, {1.0, 1.0, 1.0}, {1.0, 0, 0})
light2 = gr.light({5,2,0}, {0.5, 0.5, 0.5}, {0.7,0,0})
light3 = gr.light({20,5,0}, {0.6, 0.6, 0.6}, {1,0,0})

--from blathers.lua
light4 = gr.light({0, 200, 400.0}, {1.0, 1.0, 1.0}, {1, 0, 0})
light5 = gr.light({0,-100,-100}, {0.8, 0.8, 0.8}, {1,0,0})

gr.render(scene,
	  'museum.png', 256, 256,
	  {0, 2, 20}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {light1, light3, light4})
