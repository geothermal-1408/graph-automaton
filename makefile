all: graph gra.png

graph:graph.c
	clang -Wall -Wextra -Iraylib-5.5_macos/include graph.c raylib-5.5_macos/lib/libraylib.a -o graph -lm -lpthread -framework OpenGL -framework Cocoa -framework IOKit

render: graph
	./graph
	@echo "Processing graphs..."
	@mkdir -p png_store dot_files
	@for i in $$(seq 0 26); do \
		f="png_file$$i.dot"; \
		if [ -f "$$f" ]; then \
			neato -Tpng "$$f" -o "png_store/png_file$$i.png"; \
			mv "$$f" dot_files/; \
			echo "Moved $$f to dot_files/ and generated png_store/png_file$$i.png"; \
		fi \
	done
