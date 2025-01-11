#Step 3 - LLM reads each file and creates the JSON list of mappings

!Each! of these entries is also hashed to create a UUID "signature" based on the SHA512 of the text representation of the object.
An additional MD5 hash will be created because that's what VectorspaceDB uses so I can leverage that for RAG. will also add the model used to generate the results.

so `somefile.c` will generate `somefile.c.json`. These will be in the same dircetory structure as the original project.
