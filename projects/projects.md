# Repo specific branches

The intent is to create a branch of this repo for each specific project.  For example, there will exist a `apache/httpd` branch, a `postfix/postfix` branch, and a `mysql/mysql-server` branch along with any other common projects with public repos.  I'll create some "rollup" branches for common deployements `webserver/lamp` based on server roles... I may need some help from the community.

The core llmon_dataset repo will therefore contain no project specific output files, which feels a bit weird, but we'll see how long I keep that idea up.  I suppose I could write a script to check for installed packages and ask the user which branches to pull.

This will also allow closed-source projects to publish pull requests for their project output without exposing their actual code. Maybe this will encourage troubleshooting details from companies with less than stellar documentation.

# Project Structure

Each project here is a user/repo associated with a mapped project, for example "apache/httpd". We should probably include software versions or commits somehow.

Within each project will be a directory structure mapping to all the source code files.

Each source file in the parent project will be mapped to a related file in the JSON files

For example: somemodule.c will map to somemodule.c.log.json

***

Each file contains a JSON structure showing potential errors that might be thrown that may show up in syslog or journalctl, including the function, a prototype of the error, and 🍋-LLMon determined reason for the error. Additionally, we'll include embeddings - I hope to create a hash system to shorthand embeddings into something less numerical.

I'd also like to add potential

****

# Example with some possible future enhancements

```json:module.c.json
[
  {
    "function": "proxy_connect_handler",
    "message_template": "tunnel timed out",
    "reason": "Logs an error when the tunnel operation times out."
    "*resolution": "Possible network configuration error or incorrect host"
    "**vector":"TG9yZW0gaXBzdW0gZG9sb3Igc2l0IGFtZXQsIGNvbnNlY3RldHVyIGFkaXBpc2NpbmcgZWxpdCwgc2VkIGRvIGVpdXNtb2QgdGVtcG9yIGluY2lkaWR1bnQgdXQgbGFib3JlIGV0IGRvbG9yZSBtYWduYSBhbGlxdWEuIFV0IGVuaW0gYWQgbWluaW0gdmVuaWFtLCBxdWlzIG5vc3RydWQgZXhlcmNpdGF0aW9uIHVsbGFtY28gbGFib3JpcyBuaXNpIHV0IGFsaXF1aXAgZXggZWEgY29tbW9kbyBjb25zZXF1YXQuIER1aXMgYXV0ZSBpcnVyZSBkb2xvciBpbiByZXByZWhlbmRlcml0IGluIHZvbHVwdGF0ZSB2ZWxpdCBlc3NlIGNpbGx1bSBkb2xvcmUgZXUgZnVnaWF0IG51bGxhIHBhcmlhdHVyLiBFeGNlcHRldXIgc2ludCBvY2NhZWNhdCBjdXBpZGF0YXQgbm9uIHByb2lkZW50LCBzdW50IGluIGN1bHBhIHF1aSBvZmZpY2lhIGRlc2VydW50IG1vbGxpdCBhbmltIGlkIGVzdCBsYWJvcnVtLg0K"
  }
]
```

*resolution is something I'm trying to get the llm to do reliably, it may have to be added later in a second step

**I'm working to create a "Hash" sort of representation of an embedding vector that can be easily translated.  The placeholder text is just Base64 encoded Lorem Ipsum.

