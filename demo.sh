#!/bin/bash

# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# demo.sh - Brain Core Demo Launcher
#
# Usage: ./demo.sh
#
# Menu-driven launcher for all Brain Core demos and benchmarks
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_banner() {
    echo ""
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║                                                           ║"
    echo "║      🧠  Brain Core - Demo & Benchmark Launcher  🧠      ║"
    echo "║                                                           ║"
    echo "║        Phase 11: Portfolio Enhancement & Validation      ║"
    echo "║                                                           ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo ""
}

print_menu() {
    echo "Select an option:"
    echo ""
    echo "  [1] 🎬 Quick Start Demo (5-minute intro)"
    echo "  [2] 📊 Performance Benchmark (detailed metrics)"
    echo "  [3] 📁 View Performance Report"
    echo "  [4] 🔄 View Benchmark CSV"
    echo "  [5] 🏗️  Build all targets"
    echo "  [6] 🧹 Clean build artifacts"
    echo "  [0] ❌ Exit"
    echo ""
}

run_quickstart() {
    echo -e "${GREEN}[Running] Quick Start Demo${NC}"
    echo ""

    if [ ! -f ./demo_quickstart ]; then
        echo -e "${YELLOW}[Compiling]${NC} demo_quickstart..."
        make demo_quickstart > /dev/null 2>&1 || {
            echo -e "${RED}[Error]${NC} Compilation failed"
            return 1
        }
    fi

    ./demo_quickstart
}

run_benchmark() {
    echo -e "${GREEN}[Running] Performance Benchmark${NC}"
    echo ""

    if [ ! -f ./bench_brain_core ]; then
        echo -e "${YELLOW}[Compiling]${NC} bench_brain_core..."
        make bench_brain_core > /dev/null 2>&1 || {
            echo -e "${RED}[Error]${NC} Compilation failed"
            return 1
        }
    fi

    ./bench_brain_core
}

view_report() {
    if [ -f ./PERFORMANCE_REPORT.md ]; then
        echo -e "${GREEN}[Viewing]${NC} PERFORMANCE_REPORT.md"
        echo ""
        less PERFORMANCE_REPORT.md
    else
        echo -e "${YELLOW}[Info]${NC} PERFORMANCE_REPORT.md not found"
        echo "      Run benchmark first to generate the report"
    fi
}

view_csv() {
    if [ -f ./benchmark_results.csv ]; then
        echo -e "${GREEN}[Viewing]${NC} benchmark_results.csv"
        echo ""
        column -t -s ',' benchmark_results.csv | less
    else
        echo -e "${YELLOW}[Info]${NC} benchmark_results.csv not found"
        echo "      Run benchmark first to generate CSV"
    fi
}

build_all() {
    echo -e "${GREEN}[Building]${NC} all targets..."
    echo ""

    if ! make clean > /dev/null 2>&1; then
        echo -e "${RED}[Error]${NC} Clean failed"
        return 1
    fi

    if ! make > /dev/null 2>&1; then
        echo -e "${RED}[Error]${NC} Build failed"
        return 1
    fi

    echo -e "${GREEN}[Done]${NC} All targets built successfully"
}

clean_build() {
    echo -e "${YELLOW}[Cleaning]${NC} build artifacts..."
    echo ""

    if make clean > /dev/null 2>&1; then
        echo -e "${GREEN}[Done]${NC} Build artifacts cleaned"
    else
        echo -e "${RED}[Error]${NC} Clean failed"
        return 1
    fi
}

# Main loop
main() {
    while true; do
        print_banner
        print_menu

        read -p "Your choice [0-6]: " choice

        case $choice in
            1)
                run_quickstart
                read -p "Press Enter to continue..."
                clear
                ;;
            2)
                run_benchmark
                read -p "Press Enter to continue..."
                clear
                ;;
            3)
                view_report
                clear
                ;;
            4)
                view_csv
                clear
                ;;
            5)
                build_all
                read -p "Press Enter to continue..."
                clear
                ;;
            6)
                clean_build
                read -p "Press Enter to continue..."
                clear
                ;;
            0)
                echo ""
                echo -e "${BLUE}Thank you for using Brain Core! 🧠${NC}"
                echo ""
                exit 0
                ;;
            *)
                echo ""
                echo -e "${RED}Invalid choice. Please try again.${NC}"
                read -p "Press Enter to continue..."
                clear
                ;;
        esac
    done
}

# Run main
main
