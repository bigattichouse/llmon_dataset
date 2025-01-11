echo "Preparing workspace"
0-prep/prepare.sh $1
echo "Clone project $1..."
1-clone-project/clone.sh $1
echo "Extracting strings from $1..."
2-extract-strings/extract.sh $1
echo "Preprocess Files $1..."
3A-preprocess-files/preprocess.sh $1
echo "Prepare Prompts $1..."
3B-prepare-prompts/prepare.sh $1
echo "Begin LLM processing inference..."
4-llm-process-files/process.sh $1
echo "Generate embeddings from JSON..."
5-generate-embeddings/generate.sh $1
echo "Consolidate project data in output project/ directory"
6-consolidate-dataset/consolidate.sh $1
#echo "Clean up"
#0-prep/cleanup.sh $1

