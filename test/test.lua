local tests_run = 0
local tests_passed = 0

function assert_floats(title, as, bs)
    tests_run = tests_run + 1
    for i = 1, #as do
        if math.abs(as[i] - bs[i]) > 1e-6 then
            print("[ ] " .. title .. " : assertion failed")
            --os.exit(1)
            return
        end
    end
    print("[x] " .. title)
    tests_passed = tests_passed + 1
end

function assert_float(title, a, b)
    assert_floats(title, {a}, {b})
end

function assert_vec2(title, a, b)
    assert_floats(title, {a.x, a.y}, {b.x, b.y})
end

function assert_vec3(title, a, b)
    assert_floats(title, {a.x, a.y, a.z}, {b.x, b.y, b.z})
end

function assert_vec4(title, a, b)
    assert_floats(title, {a.x, a.y, a.z, a.w}, {b.x, b.y, b.z, b.w})
end

function test_vec2()
    assert_vec2("vec2 add", vec2(10,-10) + vec2(100,5), vec2(110,-5))
    assert_vec2("vec2 sub", vec2(10,-10) - vec2(100,5), vec2(-90,-15))

    assert_vec2("vec2 scale", 2 * vec2(2,3), vec2(4,6))
    assert_vec2("vec2 scale", vec2(2,3) * 2, vec2(4,6))

    assert_float("vec2 dot", vec2(2,3):dot(vec2(5,6)), 28)
    assert_float("vec2 cross", vec2(2,3):cross(vec2(5,6)), -3)

    assert_vec2("vec2 normalize", vec2(2,3):normalize(), vec2(2,3) * (1 / math.sqrt(13)))
    assert_float("vec2 sqr_length", vec2(2,3):sqr_length(), 13)
end

function test_vec3()
    assert_vec3("vec3 add", vec3(10,10,-10) + vec3(5,100,5), vec3(15,110,-5))
    assert_vec3("vec3 sub", vec3(10,10,-10) - vec3(5,100,5), vec3(5,-90,-15))

    assert_vec3("vec3 scale", 2 * vec3(1,2,3), vec3(2,4,6))
    assert_vec3("vec3 scale", vec3(1,2,3) * 2, vec3(2,4,6))

    assert_float("vec3 dot", vec3(1,2,3):dot(vec3(4,5,6)), 32)
    assert_vec3("vec3 cross", vec3(1,2,3):cross(vec3(4,5,6)), vec3(-3,6,-3))

    assert_vec3("vec3 normalize", vec3(1,2,3):normalize(), vec3(1,2,3) * (1 / math.sqrt(14)))
    assert_float("vec3 sqr_length", vec3(1,2,3):sqr_length(), 14)
end

function test_vec4()
    assert_vec4("vec4 add", vec4(10,10,-10,7) + vec4(5,100,5,-2), vec4(15,110,-5,5))
    assert_vec4("vec4 sub", vec4(10,10,-10,7) - vec4(5,100,5,-2), vec4(5,-90,-15,9))

    assert_vec4("vec4 scale", 2 * vec4(1,2,3,4), vec4(2,4,6,8))
    assert_vec4("vec4 scale", vec4(1,2,3,4) * 2, vec4(2,4,6,8))

    assert_float("vec4 dot", vec4(1,2,3,4):dot(vec4(5,6,7,8)), 70)

    assert_vec4("vec4 normalize", vec4(1,2,3,4):normalize(), vec4(1,2,3,4) * (1 / math.sqrt(30)))
    assert_float("vec4 sqr_length", vec4(1,2,3,4):sqr_length(), 30)
end

function test_quat()
    assert_vec4("quat add", quat(10,10,-10,7) + quat(5,100,5,-2), quat(15,110,-5,5))
    assert_vec4("quat sub", quat(10,10,-10,7) - quat(5,100,5,-2), quat(5,-90,-15,9))
    assert_vec4("quat mul", quat(1,-2,3,4) * quat(-5,6,7,-8), quat(-60,18,0,-36))

    local q0 = quat(-0.6009133, 0.3675318, 0.06183508, 0.7071068)
    local v0 = vec3(-0.7591431, -0.3433556, 0.5529997)
    local v1 = vec3(-0.1202234, 0.67126, 0.7314072)
    assert_vec3("quat mul_vec3", q0:mul_vec3(v0), v1)

    assert_vec4("quat inverse", q0 * q0:inverse(), quat())

    local s = 1 / math.sqrt(30)
    assert_vec4("quat normalize", quat(1,2,3,4):normalize(), quat(s*1,s*2,s*3,s*4))
    assert_float("quat sqr_length", vec4(1,2,3,4):sqr_length(), 30)

    local q1 = quat(0.5844759,0.1298835,-0.2597671,-0.7576539)
    local qr = quat(0.6055313,0.003286547,-0.2153544,-0.7661225)
    assert_vec4("quat slerp_to", q0:slerp_to(q1, 0.75), qr)
end

function test_mat4()
    local a = mat4()
    a.c0.x = 1; a.c1.x = 2; a.c2.x = 3; a.c3.x = 4;   
    a.c0.y = 5; a.c1.y =-6; a.c2.y =-7; a.c3.y = 8;   
    a.c0.z = 9; a.c1.z =-8; a.c2.z =-7; a.c3.z = 6;   
    a.c0.w = 5; a.c1.w = 4; a.c2.w = 3; a.c3.w = 2;   

    local b = mat4()
    b.c0.x = 2; b.c1.x = 6; b.c2.x =-8; b.c3.x = 4;   
    b.c0.y = 3; b.c1.y = 7; b.c2.y =-7; b.c3.y = 3;   
    b.c0.z = 4; b.c1.z = 8; b.c2.z =-6; b.c3.z = 2;   
    b.c0.w = 5; b.c1.w = 9; b.c2.w =-5; b.c3.w = 1;   

    local c = a * b
    assert_vec4("mat4 mul c0", c.c0, vec4(40,4,-4,44))
    assert_vec4("mat4 mul c1", c.c1, vec4(80,4,-4,100))
    assert_vec4("mat4 mul c2", c.c2, vec4(-60,4,-4,-96))
    assert_vec4("mat4 mul c3", c.c3, vec4(20,-4,4,40))

    local inv = a:inverse()
    assert_vec4("mat4 inverse c0", inv.c0, vec4(-1/18,-1/4,1/3,5/36))
    assert_vec4("mat4 inverse c1", inv.c1, vec4(-1/14,1/4,-2/7,3/28))
    assert_vec4("mat4 inverse c2", inv.c2, vec4(2/21,-1/4,3/14,-5/84))
    assert_vec4("mat4 inverse c3", inv.c3, vec4(1/9,1/4,-1/6,-1/36))
end

function run_tests()
    print("")
    test_vec2()
    print("")
    test_vec3()
    print("")
    test_vec4()
    print("")
    test_quat()
    print("")
    test_mat4()
    print("")
    print("Tests passed: " .. tests_passed .. " / " .. tests_run)
    print("")
end

run_tests()
