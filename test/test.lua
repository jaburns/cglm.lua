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
    -- TODO
end

function test_mat4()
    -- TODO
end

function run_tests()
    print("")
    test_vec2()
    print("")
    test_vec3()
    print("")
    test_vec4()
    print("")
--  test_quat()
--  print("")
--  test_mat4()
--  print("")
    print("Tests passed: " .. tests_passed .. " / " .. tests_run)
    print("")
end

run_tests()
