FROM node:20-alpine

WORKDIR /app

# Install build dependencies for native sqlite3 compilation if needed
RUN apk add --no-cache python3 make g++

COPY package*.json ./
RUN npm install --production

COPY . .

# Environment variables
ENV NODE_ENV=production
ENV PORT=3000
ENV DATA_DIR=/app/data

EXPOSE 3000
EXPOSE 1883

CMD ["node", "src/server.js"]
