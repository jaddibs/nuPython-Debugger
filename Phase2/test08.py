## test08.py ##
#
# nested loops
#
print("NESTED LOOPS")
print("")

x = True
i = 10
loop_end = i + 89

while i >= 0:
{
  while x:
  {
     print("this should happen once!")
     x = False
  }

  j = i
  j = j - 2
  print(i)
  while j <= 100:
  {
    k = "apple"
    while k != "APPLE":
    {
      var = 99
      while var != loop_end:
      {
        print('level 4 should never appear')
      }
      print(k)
      k = "APPLE"
    }
    j = j ** 2
    print(j)
  }
  print()
  i = i - 5
  x = 10 < 20
}

print('after loop:')
print(i)
print(j)
print(k)
print(var)
print(loop_end)

print("")
print("END")
