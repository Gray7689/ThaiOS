TARGET = $(BUILD_DIR)/thai-pkg

.PHONY: all clean

all:
	@mkdir -p $(BUILD_DIR)
	echo '#!/usr/bin/env python3' > $(TARGET)
	echo 'import sys; sys.path.insert(0, "/usr/lib/thai-pkg")' >> $(TARGET)
	echo 'from src.thai_pkg import main; main()' >> $(TARGET)
	chmod +x $(TARGET)

clean:
	rm -f $(TARGET)
