O = bin

CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++11 -I"tinycbor/src"

TARGET =	$(O)/TinyCBORWrapper

CBOR_SRC=$(addprefix tinycbor/src/,cborencoder.c cborencoder_close_container_checked.c cborparser.c)
CBOR_OBJ=$(addprefix $(O)/,$(notdir $(CBOR_SRC:.c=.o)))

OBJ = $(O)/MainSample.o

VPATH=bin/:tinycbor/src/:

all: $(O)/TinyCBORWrapper

.PHONY: clean

$(O)/%.o: %.cpp
	@mkdir -p ${@D}
	${CXX} -c -o $@ $< ${CXXFLAGS}
	
$(O)/%.o: %.c
	@mkdir -p ${@D}
	${CXX} -c -o $@ $< ${CXXFLAGS}

$(O)/TinyCBORWrapper: ${CBOR_OBJ} ${OBJ}
	@mkdir -p ${@D}
	${CXX} -o $@ ${OBJ} ${CBOR_OBJ} ${CXXFLAGS}

clean:
	rm -rf $(O)/

cleanall: clean
	rm -rf ${all}
