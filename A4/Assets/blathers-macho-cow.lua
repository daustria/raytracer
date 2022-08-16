-- A simple test scene featuring some spherical cows grazing
-- around Stonehenge.  "Assume that cows are spheres..."

stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.1, 0.1, 0.1}, 20)

-- #############################################
-- Read in the cow model
-- #############################################

cow_poly = gr.mesh('cow', 'cow.obj')
factor = 2.0/(2.76+3.637)

cow_poly:set_material(hide)

cow_poly:translate(0.0, 3.637, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, -1.0, 0.0)

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

-- Use the instanced cow model to place some actual cows in the scene.
-- For convenience, do this in a loop.


--for _, pt in pairs({
--		      {{1,1.3,14}, 20},
--		      {{5,1.3,-11}, 180},
--		      {{-5.5,1.3,-3}, -60}}) do

cow_instance = gr.node('cow')
scene:add_child(cow_instance)
cow_instance:add_child(cow_poly)
cow_instance:scale(1.4, 1.4, 1.4)
cow_instance:rotate('Y', 20)
cow_instance:translate(-2,1.3,3)

-- Blathers

blathers_mat = gr.material_texture('blathers.mtl')

blathers = gr.mesh( 'owl', 'blathers.obj' )
blathers:set_material(blathers_mat)
blathers:scale(0.5, 0.5, 0.5)
blathers:rotate('y',-50)
blathers:translate(2,1.3,-2)
scene:add_child(blathers)


light1 = gr.light({0, 100, 350}, {1.0, 1.0, 1.0}, {1.0, 0, 0})
light2 = gr.light({5,2,0}, {0.5, 0.5, 0.5}, {0.7,0,0})
light3 = gr.light({20,5,0}, {0.6, 0.6, 0.6}, {1,0,0})
--from blathers.lua
light4 = gr.light({0, 200, 400.0}, {1.0, 1.0, 1.0}, {1, 0, 0})
light5 = gr.light({0,-100,-100}, {0.8, 0.8, 0.8}, {1,0,0})

gr.render(scene,
	  'blathers-macho-cow.png', 512, 512,
	  {0, 2, 12}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {light1, light3, light4})
