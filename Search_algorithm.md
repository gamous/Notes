# 搜索相关算法

二进制开发中的实用算法

## 串匹配

### Sunday算法

着重关注失去匹配时的下一个字符

当模式失配时 将主串指针向后移动到模式串失配的相同字符 再向前移动失配距离 使失配位重新匹配 然后继续匹配

大幅减少失配导致的回溯，大多数情况下是足够好用的模式匹配算法



## 图搜索

### 双栈算法

全路径搜索

https://evilpan.com/2022/11/04/path-finder/

全路径本身遍历就完事了

该算法本质是对使用单栈的DFS算法的优化 但是使用辅栈来存储未经过节点 从而避免回路和爆栈

```python
#初始化栈
push(main_stack, start_node)
push(sub_stack,  start_node.adj_list)

while not main_stack.is_empty():
	if not last(sub_stack).is_empty():
	#建栈
		push(main_stack, pop(last(sub_stack)))
		push(sub_stack,  last(main_stack).adj_list.strip(main_stack))
	else:
    #削栈
         pop(main_stack)
		pop(sub_stack)
	#获取路径
	if last(main_stack)==end_node:
		paths.add(copy(main_stack))
		pop(main_stack)
		pop(sub_stack)
```



## 最短路径搜索

### Dijkstra

精确最优解算法 需要全局信息进行评估 表现在至少需要保存全节点到起始点的距离

可以理解为起点到终点的权重辐射

从起点更新权重一直比较到终点

一次遍历需要存储路径 比较费空间 如果分两次搜索 先比较最小值再获取路径则空间占用较小

```python
def dijkstra(node, node2start_dis, node2start_path):
    if node==end_node:
        return (0,[])
	updated = [(adjnode,node2start_dis + node.adj_list[adjnode] ,node2start_path.append(adjnode)) for adjnode in node.adj_list]
    forward_nodes = [ dijkstra(**forward_node) for forward_node in updated ]
    forward_nodes_dis  = [i[0] for i in forward_nodes]
    forward_nodes_path = [i[1] for i in forward_nodes]
    return forward_nodes_path[forward_nodes_dis.index(min(forward_nodes_dis))]
```

### Astar

近似解算法 根据局部信息进行启发评估 

Dijkstra+启发式搜索

启发式评估代价（heuristic estimated cost）

总是向f(n)最小方向前进

```
f(n) = g(n) + h(n)
g(n)#实际已消耗代价
h(n)#预估代价（启发函数）
f(n)#估价函数
```

### IDAstar

采用了迭代加深算法的 A * 算法

改为DFS 利于减枝 但导致回溯增加