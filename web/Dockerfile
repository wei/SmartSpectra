FROM node:18 AS build
WORKDIR /app
# backend
COPY server/package.json server/tsconfig.json server/src ./server/
RUN cd server && npm install && npm run build

FROM node:18
WORKDIR /app
COPY --from=build /app/server/dist ./dist
COPY --from=build /app/server/package.json ./package.json
COPY --from=build /app/server/node_modules ./node_modules
EXPOSE 8080
CMD ["node", "dist/index.js"]
