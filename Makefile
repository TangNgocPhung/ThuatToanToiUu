# =============================================================================
#  Makefile - Bien dich chuyen de 6 tren Linux / macOS / MSYS2 shell.
#  Tren Windows PowerShell, hay dung .\build.ps1 (khong can cai make).
#
#  Muc tieu:
#      make            bien dich tat ca vao thu muc bin/
#      make test       bien dich roi chay toan bo kiem thu
#      make stress     chay stress test lau hon
#      make bench      chay do thoi gian, ghi results/benchmark.csv
#      make debug      ban go loi: bat assert va kiem tra tran 64-bit
#      make clean      xoa san pham bien dich
# =============================================================================

CXX      ?= g++
CXXSTD   := -std=c++17
WARN     := -Wall -Wextra -Wshadow -Wconversion -pedantic
INCLUDE  := -Isrc
OPT      := -O2 -DNDEBUG
CXXFLAGS := $(CXXSTD) $(WARN) $(INCLUDE) $(OPT)

BIN      := bin
SRC      := src
TESTS    := tests

HEADERS  := $(SRC)/segment_tree.hpp $(SRC)/segment_tree_ext.hpp $(SRC)/naive.hpp

TARGETS  := $(BIN)/segtree $(BIN)/bench $(BIN)/generator \
            $(BIN)/stress_test $(BIN)/unit_tests $(BIN)/unit_tests_checked

.PHONY: all test stress bench debug clean help

all: $(TARGETS)
	@echo "Bien dich xong. Cac tep thuc thi nam trong $(BIN)/"

$(BIN):
	@mkdir -p $(BIN)

$(BIN)/segtree: $(SRC)/main.cpp $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/bench: $(SRC)/bench.cpp $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/generator: $(TESTS)/generator.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/stress_test: $(TESTS)/stress_test.cpp $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN)/unit_tests: $(TESTS)/unit_tests.cpp $(HEADERS) | $(BIN)
	$(CXX) $(CXXFLAGS) $< -o $@

# Ban kiem thu co bat assert va kiem tra tran 64-bit.
$(BIN)/unit_tests_checked: $(TESTS)/unit_tests.cpp $(HEADERS) | $(BIN)
	$(CXX) $(CXXSTD) $(WARN) $(INCLUDE) -O1 -g -DST_CHECK_OVERFLOW $< -o $@

test: $(BIN)/unit_tests $(BIN)/unit_tests_checked $(BIN)/segtree
	@echo "=== Kiem thu don vi (ban toi uu) ==="
	@$(BIN)/unit_tests
	@echo ""
	@echo "=== Kiem thu don vi (ban co ST_CHECK_OVERFLOW) ==="
	@$(BIN)/unit_tests_checked
	@echo ""
	@echo "=== Test co dinh trong $(TESTS)/cases ==="
	@fail=0; total=0; \
	for f in $(TESTS)/cases/*.in; do \
	    exp=$${f%.in}.out; \
	    [ -f "$$exp" ] || continue; \
	    total=$$((total+1)); \
	    $(BIN)/segtree "$$f" /tmp/st_out.txt; \
	    if diff -q "$$exp" /tmp/st_out.txt > /dev/null 2>&1; then \
	        echo "  [ OK ] $$(basename $$f)"; \
	    else \
	        echo "  [FAIL] $$(basename $$f)"; fail=$$((fail+1)); \
	    fi; \
	done; \
	echo "  Ket qua: $$((total-fail))/$$total test co dinh dat"; \
	rm -f /tmp/st_out.txt; \
	[ $$fail -eq 0 ]

stress: $(BIN)/stress_test
	$(BIN)/stress_test --rounds=5000 --maxn=200 --ops=500 --seed=1

bench: $(BIN)/bench
	@mkdir -p results
	$(BIN)/bench --sizes=1000,10000,100000,1000000 --ops=200000 --repeat=5 \
	             --with-naive --out=results/benchmark.csv

debug: CXXFLAGS := $(CXXSTD) $(WARN) $(INCLUDE) -O1 -g -DST_CHECK_OVERFLOW
debug: clean all
	@echo "Da bien dich ban go loi (assert + kiem tra tran 64-bit)."

clean:
	rm -rf $(BIN)

help:
	@echo "make          bien dich tat ca"
	@echo "make test     chay toan bo kiem thu"
	@echo "make stress   chay stress test 5000 vong"
	@echo "make bench    do thoi gian, ghi results/benchmark.csv"
	@echo "make debug    ban go loi (assert + kiem tra tran 64-bit)"
	@echo "make clean    xoa thu muc bin/"
