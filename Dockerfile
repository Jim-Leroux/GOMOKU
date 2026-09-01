# ==========================================
# STAGE 1: Build WebAssembly (C++ -> WASM)
# ==========================================
FROM emscripten/emsdk AS wasm-builder

WORKDIR /src
COPY . .

# Compile WASM directly
RUN em++ -std=c++17 -O3 -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME="createGomoku" --bind \
    -Iinclude \
    src/Board.cpp src/GameEngine.cpp src/WasmBindings.cpp \
    -o gomoku.js

# ==========================================
# STAGE 2: Build React App (Vite)
# ==========================================
FROM node:20-alpine AS frontend-builder

WORKDIR /app
COPY web/package.json web/package-lock.json* ./
RUN npm install

# Copy all frontend files
COPY web/ ./

# Copy WASM files from stage 1 to public folder
COPY --from=wasm-builder /src/gomoku.wasm /src/gomoku.js ./public/

# Build the React app
RUN npm run build

# ==========================================
# STAGE 3: Serve with Nginx
# ==========================================
FROM nginx:alpine

# Copy custom Nginx configuration
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Copy React build to Nginx serving directory
COPY --from=frontend-builder /app/dist /usr/share/nginx/html

EXPOSE 8080
CMD ["nginx", "-g", "daemon off;"]
