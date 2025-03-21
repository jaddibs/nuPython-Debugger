## test07.py ##
#
# nested loops
#
print("NESTED LOOPS")
print("")

i = 1

while i != 5:
{
  j = i
  j = j + 1
  
  while j <= 7:
  {
     print("j")  
     print(j)
     j = j + 1
  }

  i = i + 1
  print("")

  while False:
  {
     print("this should never happen!")
     print("this should never happen!")
     print("this should never happen!")
  }
  
  k = i
  while k > 2:
  {
      print("k")  
      print(k)
      k = k - 1
  }
  
  print("")
}

print(i)
print(j)
print(k)

print("")
print("END")
