docker run --rm -v "$($pwd.Path):/work" -v "$($pwd.Path)/.tex-mf-var:/root/.texmf-var" vvakame/review:latest /bin/sh -c "cd /work && rake clean pdf"
