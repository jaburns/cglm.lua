
function printvec(v)
    print(v.x .. " " .. v.y .. " " .. v.z)
end

printvec(vec3(10,10,10) + vec3(5,100,5))

v0 = vec3(2, 3, 4)
v1 = quat.angle_axis(math.pi, vec3(0,1,0)):mul_vec3(v0)

printvec(v0)
printvec(v1)


--[[
local vec3_instance_meta_ = {
    __add = function(l, r)
        return vec3(l.x + r.x, l.y + r.y, l.z + r.z)
    end,
    __index = {
        hello = function(l, r)
            return "instance function hello"
        end
    }
}
local vec3_static_meta_ = {
    __call = function(t, x1, y1, z1)
        return setmetatable({
            x = x1,
            y = y1,
            z = z1
        }, vec3_instance_meta_)
    end,
    __index = {
        doge = function()
            return "static function doge"
        end
    }
}
vec3 = setmetatable({}, vec3_static_meta_)

vx = vec3(1,2,3) + vec3(4,5,6)
vx:hello(vec3(1,1,1))
vx.doge()



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
