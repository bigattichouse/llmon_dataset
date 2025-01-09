#Project Files

Each project here is a user/repo associated with a mapped project, for example "apache/httpd". We should probably include software versions or commits somehow.

Within each project will be a directory structure mapping to all the source code files.

Each source file in the parent project will be mapped to a related file in the JSON files

For example: somemodule.c will map to somemodule.c.log.json

***

Each file contains a JSON structure showing potential errors that might be thrown that may show up in syslog or journalctl, including the function, a prototype of the error, and 🍋-LLMon determined reason for the error. Additionally, we'll include embeddings - I hope to create a hash system to shorthand embeddings into something less numerical.


