TARGET = $(BUILD_DIR)/thai-browser

.PHONY: all clean

all:
	@mkdir -p $(BUILD_DIR)
	echo '#!/usr/bin/env python3' > $(TARGET)
	echo 'import sys; sys.path.insert(0, "/usr/lib/ThaiBrowser")' >> $(TARGET)
	echo 'from src.main import main; main()' >> $(TARGET)
	chmod +x $(TARGET)

install:
	install -Dm755 $(TARGET) $(DESTDIR)/usr/bin/thai-browser
	install -Dm644 src/main.py $(DESTDIR)/usr/lib/ThaiBrowser/src/main.py

clean:
	rm -f $(TARGET)
