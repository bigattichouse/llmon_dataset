#Step 4 - Now we cycle through all the JSON entries created and create an embedding for each.

These embeddings get saved in `somefile.c.embedding` as binhex, as a pip3 deliminated list of Base64 encoded values.
This will have the MD5 hash and signature of the original JSON object,  the embedding engine used and the valyes.
