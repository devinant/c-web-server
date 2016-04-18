OUT_DIR = dv1457-httpd
OUT = $(OUT_DIR).tgz
SRC_DIR = webserver

all:
	make -C $(SRC_DIR)
	
clean:
	make clean -C $(SRC_DIR)

package:
	-rm -rf $(OUT_DIR)
	-rm $(OUT)
	make -C $(SRC_DIR)/
	make clean -C $(SRC_DIR)/
	mkdir $(OUT_DIR) && rsync -a * $(OUT_DIR)/ --exclude $(OUT_DIR)/ --exclude $(SRC_DIR)/objects --exclude $(SRC_DIR)/.idea --exclude .git/
	-rm $(OUT_DIR)/.gitignore $(OUT_DIR)/INSTALL.md $(OUT_DIR)/README.md $(OUT_DIR)/Makefile
	tar cvzf $(OUT) $(OUT_DIR)/
	rm -rf $(OUT_DIR)
