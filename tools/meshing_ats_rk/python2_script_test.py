import urllib.request, urllib.error, urllib.parse

try:
    x = urllib.request.urlopen("https://pythonprogramming.net").read()
    
    print(x)

except Expception as e:
    print(str(e))
