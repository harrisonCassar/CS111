# NAME: Harrison Cassar
# EMAIL: Harrison.Cassar@gmail.com
# ID: 505114980

# default build
default:
	chmod 766 lab3b.sh
	ln lab3b.sh lab3b
	echo "Default build successful."

# build the distribution tarball
dist:
	tar -zvcf lab3b-505114980.tar.gz Makefile README lab3b.sh lab3b.py

# delete all files created by the Makefile, and return the directory to its freshly untared state
clean:
	rm -f *~ *.tar.gz lab3b

.PHONY: clean dist default
.SILENT: default dist clean