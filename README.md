# 🍋-llmon dataset
Open synthetic dataset for parsing syslog and journalctl output, created by analysis of open project repos. Intended to be used in https://github.com/bigattichouse/llmon to provide RAG and context information about log messages in realtime `journalctl` and `syslog` analysis.

# Primary purpose
Dataset will be used for Retrieval Augmented Generation (RAG) for the 🍋-LLmon system monitor.
Additionally, this dataset may be used to fine-tune an LLM to be better suited to processing linux syslog and journalctl entries and provide diagnostic opinions on how to optimize linux systems and post-mortem system failures.

# Data Updates
By separating the dataset from the main codebase, it will be possible to use `git` as a simple distribution mechanism for source updates.

# Design Philosophy
Reading and analyzing errors in system logs is not trivial and requires a lot of general knowledge about how software works.
Since most linux software is based on open source, why not use those project public git repos and anaylze the code base for instances of thrown errors and logged information. By having an LLM analyze all the individual models in the codebase, we can create a large knowledgebase of potential log messages and the original intent of those messages. Then, at run time, we can use RAG to locate relevant information and use that intermixed with the actual log messages to allow a small reasoning LLM to give us a summary of events and potential changes to be made.

In all things, I hoped to keep this tool as dead simple as possible - using bash and C (or C++) to run with as few external requirements as possible. This should follow the unix philosophy of a single small tool to do each job well. It's definitely helped me create the pipeline using AI - since the LLMs can focus on one discrete task at a time.

Example Summary and log information extracted for later use in RAG:
```json
{
  "project": "sendmail",
  "module": "../working/sendmail/sendmail/trace.c",
  "summary": "This module implements trace flag handling for sendmail's debugging system. It parses both old-style (numeric) and new-style (pattern-based) trace flags and registers them with the debugging system, controlling what debug information gets logged elsewhere in the application.",
  "logs": [
    {
      "function": "tTnewflag",
      "message_template": "sm_debug_addsetting_x registration",
      "reason": "Registers a debug setting with the specified pattern and level. This controls what will be logged elsewhere in the application when debug tracing is enabled.",
      "resolution": "This is not a direct log message but a configuration action. If trace-related issues occur, check the trace flag syntax in configuration files or command line arguments. Valid formats include numeric flags, ranges, and pattern-based flags with optional level specifications."
    }
  ]
}
```

# Build Process
You should not need to build projects (apache/httpd), they should all be available in the `projects/` directory. The build process is only included in case you wish to add your own project to the dataset and initiate a pull request to have your project accessible to everyone. It's also nice if people want to help improve the build logic and pipeline when such tools are created.

Initial tests building the project with local LLMs took DAYS, even for small projects. The project received a small grant from a private donor to build data using anthropic's claude 3.7.

# Branches

Thinking of maybe doing branches for each major project Apache, Mysql, Sendmail, etc.  Then I could do packages like LAMP (Apache/Mysql/PHP) as branches.  This way you could pull/merge what you want without needing everything else.  For the initial version, I'm just going to use the main branch, and then we'll figure out from there. I realize major software versions (knowing leg entries for multiple versions of mysql) could make this unweildy, but I wanted a dataset to build the llmon MVP.

# TODO  
1. The `jq` parsing isn't quite what I need, I get `./summarize.sh: line 30: /usr/bin/jq: Argument list too long` errors, which aren't good. For now it's fine, but I'm sure I'm missing important messages. I need to better handle parsing the JSON.
2. Need to implement chunking to handle larger files (related to #1 above). 128k context at claude helped fix this except for a few files in the batch.. files with `null` data are likely errored files that need to be fixed later.
3. Summaries should probably be much shorter and be relevant to troubleshooting, maybe like a bullet list of 5-10 items about the module and how it might cause problems in logs.. this is likely just prompt engineering.
4. abstract the inference steps to use ollama/openai/llama.cpp/kobold or whatever people want to use.

For now though - I just want to build the minimum viable product.
