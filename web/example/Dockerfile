FROM node:18-alpine AS build
WORKDIR /app
COPY package*.json tsconfig.json vite.config.ts ./
RUN npm install
COPY src ./src
COPY index.html ./
RUN npm run build

FROM node:18-alpine
WORKDIR /app
COPY --from=build /app/dist ./dist
RUN npm install -g serve
EXPOSE 3000
CMD ["serve", "-s", "dist", "-l", "3000"]
