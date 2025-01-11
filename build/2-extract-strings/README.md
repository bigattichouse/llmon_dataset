#Step 2 - Run code to extract strings from working files.

The LLMs can wander off if you don't use faster parsing techniques to identify a list of strings for it to pay attention to.
By pointing out these strings, it seems to keep it on task of identifying potential log entries (which are also usually strings)

Note that it should search header and source files in case the C files make use of constants. I may need to include headers and an extra step to map them if this becomes a problem. That can be a problem to solve when I run into it.

For now, this step is just the brute force extraction of strings related to a file, and I can easily expand that to include headers.

Example:

```
In the upcoming file, note the following important strings which will need log entries, some may be comments and should be ignored:

String Line 5:"License"
String Line 11:"AS IS"
String Line 19:"mod_proxy.h"
String Line 20:"apr_poll.h"
String Line 32:"Tunneling SSL Through a WWW Proxy"
String Line 94:"AllowCONNECT: port numbers must be numeric"
String Line 107:"Cannot parse '%s' as port number"
String Line 141:"canonicalising URL %s"
String Line 177:"declining URL
.
.
.
[long list of possible strings]


Please analyze the following snippet:

[file goes here]


``` 


