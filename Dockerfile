FROM node:20-alpine

WORKDIR /app

# Install build dependencies for sqlite3 native addon
RUN apk add --no-cache python3 make g++

COPY backend/package*.json ./
RUN npm install --omit=dev

COPY backend/ ./

ENV NODE_ENV=production \
    PORT=3000

EXPOSE 3000

CMD ["node", "src/server.js"]
