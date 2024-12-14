(** QUESTION 5: Implement insert_sorted [4 marks]
This function inserts an element into a sorted list while
maintaining ascending order
Input:
- cmp: comparison function of type ('a -> 'a -> int) that
returns:
negative if first arg < second arg
zero if args are equal
positive if first arg > second arg
- x: element to insert
- lst: sorted input list
Returns:
- a new sorted list with x inserted in the correct
position
Examples:
insert_sorted compare 2 (Node(1, Node(3, Empty)))
(* returns Node(1, Node(2, Node(3, Empty))) because 2
belongs between 1 and 3 *)
insert_sorted String.compare "b" (Node("a", Node("c",
Empty)))
(* returns Node("a", Node("b", Node("c", Empty))) because
"b" belongs between "a" and "c" *)
Hint: Use pattern matching, recursion, and comparison
function *)
let rec insert_sorted (cmp: 'a -> 'a -> int) (x: 'a) (lst: 'a
my_list) : 'a my_list =
match lst with
| Empty -> Node(x, Empty)
| Node(y, rest) -> if cmp x y < 0 then Node(x, Node(y,
rest)) else(** QUESTION 6: Implement my_sort [4 marks]
This function sorts a list in ascending order using a
comparison function
Input:
- cmp: comparison function of type ('a -> 'a -> int) that
returns:
negative if first arg < second arg
zero if args are equal
positive if first arg > second arg
- lst: input list to sort
Returns:
- a new list with all elements sorted according to the
comparison function
Examples:
my_sort compare (Node(3, Node(1, Node(2, Empty))))
(* returns Node(1, Node(2, Node(3, Empty))) because 1 < 2
< 3 *)
my_sort (fun x y -> compare (String.length x)
(String.length y))
(Node("abc", Node("a", Node("ab", Empty))))
(* returns Node("a", Node("ab", Node("abc", Empty)))
because lengths: 1 < 2 < 3 *)
Hint: Use pattern matching, insert_sorted, and recursion
*)
let rec my_sort (cmp: 'a -> 'a -> int) (lst: 'a my_list) : 'a
my_list =
match lst with
| Empty -> Empty
(* | Node(x, Empty) -> Node(x, Empty) *)
| Node(x, rest) -> insert_sorted cmp x (my_sort cmp rest)(** 
