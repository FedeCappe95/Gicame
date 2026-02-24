##############################################################################
# Compiler and tools
##############################################################################

CXX          := g++
FLAGS        := -DGICAME_EXPORTS -I./headers -I./internal_headers -lstdc++ -L/usr/lib -L./build
FLAGS_CRYPTO := -DGICAME_CRYPTO_EXPORTS -I./headers -I./internal_headers -lstdc++ -L/usr/lib -L./build -I/usr/lib/openssl-1.0/include -L/usr/lib/openssl-1.0/lib -lssl -lcrypto
COMMON_FLAGS := -std=c++17 -fPIC -Wall -Wextra -lrt
RELEASEFLAGS := -O2
DEPFLAGS     := -MMD -MP
AR           := ar
ARFLAGS      := rcs


##############################################################################
# Build directories
##############################################################################

OS        := $(subst /,-,$(shell uname -o))
BUILD_DIR := build-$(OS)
SRC_DIR   := src
BIN_DIR   := bin


##############################################################################
# Source discovery
##############################################################################

SRC        := $(shell find $(SRC_DIR) -name '*.cpp' ! -path 'src/crypto/*')
SRC_CRYPTO := $(shell find $(SRC_DIR)/crypto -name '*.cpp')
OBJ        := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
OBJ_CRYPTO := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_CRYPTO))
DEP        := $(OBJ:.o=.d) $(OBJ_CRYPTO:.o=.d)


##############################################################################
# Examples discovery
##############################################################################

EXAMPLES_DIR := examples
EXAMPLE_DIRS := $(sort $(dir $(wildcard $(EXAMPLES_DIR)/*/)))
EXAMPLE_SRC  := $(shell find $(EXAMPLES_DIR) -name '*.cpp')
EXAMPLE_OBJ  := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(EXAMPLE_SRC))
EXAMPLE_DEP  := $(EXAMPLE_OBJ:.o=.d)
# Executable name = directory name
EXAMPLE_BINS := $(patsubst $(EXAMPLES_DIR)/%/,$(BIN_DIR)/%,$(EXAMPLE_DIRS))


##############################################################################
# Targets
##############################################################################

all: $(BIN_DIR)/libgicame.so $(BIN_DIR)/libgicame.a $(BIN_DIR)/libgicamecrypto.so $(BIN_DIR)/libgicamecrypto.a examples
	@echo Done

$(BIN_DIR)/libgicame.so: $(OBJ)
	@echo Building $@
	@mkdir -p $(dir $@)
	@$(CXX) $^ $(COMMON_FLAGS) $(FLAGS) -shared -o $@

$(BIN_DIR)/libgicame.a: $(OBJ)
	@echo Building $@
	@mkdir -p $(dir $@)
	@$(AR) $(ARFLAGS) $@ $^

$(BIN_DIR)/libgicamecrypto.so: $(OBJ_CRYPTO)
	@echo Building $@
	@mkdir -p $(dir $@)
	@$(CXX) $^ $(COMMON_FLAGS) $(FLAGS_CRYPTO) -shared -o $@

$(BIN_DIR)/libgicamecrypto.a: $(OBJ_CRYPTO)
	@echo Building $@
	@mkdir -p $(dir $@)
	@$(AR) $(ARFLAGS) $@ $^

examples: $(EXAMPLE_BINS)
	@echo Examples done


##############################################################################
# Generic compilation rules (Gicame)
##############################################################################

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Compiling $<
	@mkdir -p $(dir $@)
	@$(CXX) $(COMMON_FLAGS) $(RELEASEFLAGS) $(FLAGS) $(DEPFLAGS) -c $< -o $@


##############################################################################
# Generic compilation rules (Gicame-crypto)
##############################################################################

$(BUILD_DIR)/crypto/%.o: $(SRC_DIR)/crypto/%.cpp
	@echo Compiling crypto $<
	@mkdir -p $(dir $@)
	@$(CXX) $(COMMON_FLAGS) $(RELEASEFLAGS) $(FLAGS_CRYPTO) $(DEPFLAGS) -c $< -o $@


##############################################################################
# Generic compilation rules (examples)
##############################################################################

$(BUILD_DIR)/examples/%.o: examples/%.cpp
	@echo Compiling example $<
	@mkdir -p $(dir $@)
	@$(CXX) $(COMMON_FLAGS) $(RELEASEFLAGS) $(FLAGS) $(DEPFLAGS) -c $< -o $@


##############################################################################
# Link examples
##############################################################################

define LINK_EXAMPLE
$(BIN_DIR)/$(1): $$(filter $(BUILD_DIR)/examples/$(1)/%.o,$$(EXAMPLE_OBJ)) \
                 $(BIN_DIR)/libgicame.so \
                 $(BIN_DIR)/libgicamecrypto.so
	@echo Linking example $(1)
	@mkdir -p $$(dir $$@)
	@$$(CXX) $$^ $(COMMON_FLAGS) $(FLAGS) $(FLAGS_CRYPTO) \
	    -Wl,-rpath,'$$ORIGIN' \
	    -o $$@
endef

$(foreach ex,$(notdir $(EXAMPLE_DIRS:/=)),$(eval $(call LINK_EXAMPLE,$(ex))))


##############################################################################
# Auto dependencies
##############################################################################

-include $(DEP)

##############################################################################
# Clean
##############################################################################

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)