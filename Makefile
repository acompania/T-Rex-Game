all: client

client:
	@echo "================= Building Client ================="
	cd client_src; make

clean:
	cd client_src; make clean
