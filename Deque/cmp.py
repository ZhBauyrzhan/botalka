# read arrays from file
with open("tmp.txt") as f:
    arr1 = list(map(int, f.readline().split()))
    arr2 = list(map(int, f.readline().split()))

if arr1 == arr2:
    print("Arrays are identical")
else:
    print("Arrays are different")

print(len(arr1), len(arr2))

# show element-wise differences
for i, (a, b) in enumerate(zip(arr1, arr2)):
    if a != b:
        print(f"Difference at index {i}: {a} != {b}")
