FROM node:18 AS build
WORKDIR /app
# build sdk
COPY sdk/package.json sdk/tsconfig.json sdk/src ./sdk/
RUN cd sdk && npm install && npm run build
# build example
COPY example/package.json example/tsconfig.json example/index.html example/src ./example/
RUN cd example && npm install && npm run build

FROM node:18
WORKDIR /app
COPY --from=build /app/example/dist ./dist
RUN npm install -g serve
EXPOSE 5173
CMD ["serve", "-s", "dist", "-l", "5173"]
