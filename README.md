# parallels
Tournament tree A simple principle to reduce the number of shared memory
accesses is to use a tournament tree. Such a tree is a complete binary tree. To simplify
the presentation, we consider that the number of processes is a power of 2, i.e., n = 2k
(hence k = log2 n). If n is not a power of two, it has to be replaced by n
 = 2k where
k = 
log2 n (i.e., n
 is the smallest power of 2 such that n

> n).
Such a tree for n = 23 processes p1, . . . , p8, is represented in Fig. 2.10. Each
node of the tree is any two-process starvation-free mutex algorithm, e.g., Peterson’s
two-process algorithm. It is even possible to associate different two-process mutex
algorithms with different nodes. The important common feature of these algorithms
is that any of them assumes that it is used by two processes whose identities are 0
and 1.
As we have seen previously, any two-process mutex algorithm implements a lock
object.Hence, we consider in the following that the tournament tree is a tree of (n−1)
locks and we accordingly adopt the lock terminology. The locks are kept in an array
denoted LOCK[1..(n−1)], and for x = y, LOCK[x] and LOCK[y] are independent
objects (the atomic registers used to implement LOCK[x] and the atomic registers
used to implement LOCK[y] are different).
The lock LOCK[1] is associated withe root of the tree, and if it is not a leaf, the
node associated with the lock LOCK[x] has two children associated with the locks
LOCK[2x] and LOCK[2x + 1].
According to its identity i , each process pi starts competing with a single other
process p j to obtain a lock that is a leaf of the tree. Then, when it wins, the process
pi proceeds to the next level of the tree to acquire the lock associated with the node
that is the father of the node currently associated with pi (initially the leaf node
associated with pi ). Hence, a process competes to acquire all the locks on the path
from the leaf it is associated with until the root node.
As (a) the length of such a path is 
log2 n and (b) the cost to obtain a lock
associated with a node is O(1) in contention-free scenarios, it is easy to see that
the number of accesses to atomic registers in these scenarios is O(log2 n) (it
is exactly 4 log2 n when each lock is implemented with Peterson’s two-process
algorithm).
Parallels of Popova Mariia 
