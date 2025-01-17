
CC = clang
CFLAGS = -Wextra -Wall -std=gnu99 -Iinclude -Wno-unused-parameter -Wno-unused-variable -Wno-duplicate-decl-specifier

MQTT_C_SOURCES = src/mqtt.c src/mqtt_pal.c
MQTT_C_EXAMPLES = bin/simple_publisher bin/simple_subscriber bin/mqttpublish
BINDIR = bin

all: $(BINDIR) $(MQTT_C_EXAMPLES)

bin/mqttpublish: examples/mqttpublish.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) $^ -lpthread -o $@

bin/simple_%: examples/simple_%.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) $^ -lpthread -o $@

bin/reconnect_%: examples/reconnect_%.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) $^ -lpthread -o $@

bin/bio_%: examples/bio_%.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) `pkg-config --cflags openssl` -D MQTT_USE_BIO $^ -lpthread `pkg-config --libs openssl` -o $@

bin/openssl_%: examples/openssl_%.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) `pkg-config --cflags openssl` -D MQTT_USE_BIO $^ -lpthread `pkg-config --libs openssl` -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(BINDIR)
