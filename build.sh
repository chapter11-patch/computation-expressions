docker run --rm -v $(pwd):/work -v $(pwd)/.tex-mf-var:/root/.texmf-var \
  vvakame/review:latest /bin/sh -c "cd /work && rake clean lint pdf"
