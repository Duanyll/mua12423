# 关于函数和作用域实现

首先对于全局变量, 由于 `_G` 的关系, 须放在一个 `table` 中, 因此不与局部变量共同处理.

在 Lua 解释器中运行结果表明, Lua 中局部函数对外部变量的捕获是按引用进行的.

但是每次函数调用会创建一个新的作用域.

```lua
function fun(a) 
    return function(b) a * b end
end

x = fun(2)
y = fun(3)
x(4) -- 4 * 2 = 8
y(5) -- 3 * 5 = 15
```

可见对同一个函数的多次调用, 其内部的 lambda 表达式捕获到的是不同变量的引用.

因此在实现上, 需要分为两层: 栈帧(frame)和作用域(scope), frame 在函数调用时被创建并且具有唯一标识符, scope 用于处理局部变量重名的问题.

- `local_var_id`: 编译时确定的变量 ID, 解决同一个 frame 内部不同 scope 中变量重名的问题.
- `storage_id`: 运行时分配的变量 ID, 解决按引用捕获的问题, `ast_function` 对象中存储被捕获的变量的 `storage_id`, 以便按引用捕获, 进入 frame 时, 向 `context` 提供需要捕获的 `local_var_id` 对应的 `storage_id`.

- `capture_varible(storage_id)` 方法