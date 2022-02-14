FROM mysql:latest

LABEL Name=collector_db

ENV MYSQL_ROOT_PASSWORD=root

COPY ./mysqlsampledatabase.sql /docker-entrypoint-initdb.d/schema.sql