FROM node:lts-slim AS base

ARG APP_USER_HOME=/root/home
WORKDIR $APP_USER_HOME

ENV PATH $APP_USER_HOME/node_modules/.bin:$PATH

COPY package.json $APP_USER_HOME/package.json
RUN npm install

FROM base AS development
WORKDIR $APP_USER_HOME/frontend
COPY . $APP_USER_HOME