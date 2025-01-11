#Step 3 - LLM reads each file and creates the JSON list of mappings

This step is a placeholder for when I need to reconcile things like literal strings in headers with the associated source code.
I'll have to find constants assigned to string values, then look those up in an array if they are in a given source file, 
and then add a sub prompt "When you see the following constants, use the following strings instead .." just like I'm doing with the "Pay special attention to the following strings" extracted in step 2.
