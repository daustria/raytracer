stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
wood = gr.material({0.59, 0.29, 0.2}, {0.1, 0.1, 0.1}, 5) --for the frames 
wall = gr.material({1.0, 0.1, 0.4}, {0.0, 0.0, 0.0}, 10 )
bone = gr.material({0.89, 0.85, 0.78}, {0.0, 0.0, 0.0}, 5)
marble = gr.material({0.89, 0.88, 0.8}, {0.0, 0.0, 0.0}, 50)
light_mat = gr.material({1.0, 1.0, 0}, {0, 0, 0}, 2)

scene = gr.node('scene')

-- floor
wood_flooring = gr.material_texture('scene_floor.mtl')

floor = gr.mesh('floor', 'scene_floor.obj')
floor:set_material(wood_flooring)
floor:translate(0,0,1.0)

scene:add_child(floor)

-- left wall
lwall = gr.mesh('lwall', 'scene_lwall.obj')
lwall:set_material(wall)
lwall:scale(1.0,2.0,1.0)
scene:add_child(lwall)

-- rwall
rwall = gr.mesh('rwall', 'scene_rwall.obj')
rwall:set_material(wall)
rwall:scale(1.0,2.0,1.0)
scene:add_child(rwall)

-- add the paintings

-- venus
venus_frame = gr.mesh('venus_frame', 'scene_venus_frame.obj')
venus_frame:set_material(wood)

venus = gr.mesh('venus', 'scene_venus.obj')
venus:set_material(gr.material_texture('scene_venus.mtl'))

venus_frame:add_child(venus)

venus_painting = gr.node('venus_painting')
venus_painting:translate(0,1.0,0)
venus_painting:add_child(venus_frame)

scene:add_child(venus_painting)

-- summer
summer_frame = gr.mesh('summer_frame', 'scene_summer_frame.obj')
summer_frame:set_material(wood)
summer = gr.mesh('summer', 'scene_summer.obj')
summer:set_material(gr.material_texture('scene_summer.mtl'))

summer_painting = gr.node('summer_painting')
summer_painting:add_child(summer_frame)
summer_frame:add_child(summer)

scene:add_child(summer_painting)

-- mona
mona_frame = gr.mesh('mona_frame', 'scene_mona_frame.obj')
mona_frame:set_material(wood)

mona = gr.mesh('mona', 'scene_mona.obj')
mona:set_material(gr.material_texture('scene_mona.mtl'))

mona_painting = gr.node('mona_painting')
mona_painting:add_child(mona_frame)
mona_frame:add_child(mona)

mona_painting:translate(-5.4,0.0,-6.1)
mona_painting:translate(-1.0,0.5,-1.0)
scene:add_child(mona_painting)


-- now add the dinosaur fossil
fossil = gr.mesh('fossil', 'scene_dino.obj')
fossil:set_material(bone)
fossil_stand = gr.mesh('dino', 'scene_dino_stand.obj') -- i can make this a box to speed up 
fossil_stand:set_material(stone)

dinosaur = gr.node('dino_fossil')
dinosaur:add_child(fossil)
dinosaur:add_child(fossil_stand)

-- This fossil is a lot of vertices so I comment this out if I am just test rendering a scene
-- scene:add_child(dinosaur)

-- random sphere
--
-- Perhaps I can make this a glass sphere one day 
sphere = gr.nh_sphere('sphere', {-6.4, 1.0, -16.0}, 1.0)
sphere:set_material(gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25))
sphere:translate(9.0,0,4.5) -- I want it here now ... 
scene:add_child(sphere)

-- some seats

seat1 = gr.nh_box('seat1', {0.0, 0.0, 0.0}, 1.0)
seat1:set_material(stone)
seat1:rotate('Y',45)
seat1:translate(-2.8, 0, -10.7)

seat2 = gr.nh_box('seat2', {0.0, 0.0, 0.0}, 1.0)
seat2:set_material(stone)
seat2:rotate('Y',45)
seat2:translate(-1.8, 0, -11.7)

seats = gr.node('seats')
seats:add_child(seat1)
seats:add_child(seat2)

-- I want to see the reflection on the floor a bit more, so move the seats out of the way
seats:translate(1.5,0,2.0)

scene:add_child(seats)

-- add a statue
nike = gr.mesh('nike', 'scene_nike.obj')
nike:set_material(marble)
nike:translate(1.0,0.0,3.0)
-- not sure if i want to include this. scene is getting too cluttered and i may want fishy
-- scene:add_child(nike) 

-- add blathers
blathers = gr.mesh('blathers', 'scene_blathers.obj')
blathers:set_material(gr.material_texture('blathers.mtl'))

scene:add_child(blathers)

-- light
light1 = gr.light({0, 9, -18}, {0.7, 0.7, 0.7}, {1,0,0})
light2 = gr.light({5,9,-18}, {0.6, 0.6, 0.6}, {1,0,0})
light3 = gr.light({-3,5,-21}, {0.7, 0.7, 0.7}, {1,0,0})

-- these are here to light up blathers
light4 = gr.light({2, 5, -9}, {0.6, 0.6, 0.6}, {1.0, 0, 0})
light5 = gr.light({-1, 4, -12}, {0.6, 0.6, 0.6}, {1.0, 0, 0})

gr.render(scene,
	  'scene.png', 200, 200,
	  {0, 4, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.2, 0.2, 0.2}, {light1, light4})
