
function printvec(v)
    print(v.x .. " " .. v.y)
end

v0 = vec2(2, 3)
v1 = vec2(10, 20)

print(tostring((v1 * 5).x))


--[[
local v3_instance_meta_ = {
    __add = function(l, r)
        return v3(l.x + r.x, l.y + r.y, l.z + r.z)
    end,
    __index = {
        cross = function(l, r)
            -- instance function cross
            return v3(99, 99, 99)
        end
    }
}

local v3_static_meta_ = {
    __call = function(t, x1, y1, z1)
        return setmetatable({
            x = x1,
            y = y1,
            z = z1
        }, v3_instance_meta_)
    end,
    __index = {
        doge = function()
            return "static function doge"
        end
    }
}
v3 = setmetatable({}, v3_static_meta_)
vx = v3(1,2,3) + v3(4,5,6)
print(vx:cross(v3(1,2,3)).x .. "")



function print_q(v)
    print(v.x.." "..v.y.." "..v.z.." "..v.w)
end

q0 = quat()
print_q(q0)

v0 = vec3(1,2,3)
v1 = vec3(10,20,30)
v3 = v0 + v1
v = v3 + v0
print(v.x.." "..v.y.." "..v.z)
]]
