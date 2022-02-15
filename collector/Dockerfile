FROM python:3

LABEL Name=collector_py

WORKDIR /usr/src/app

COPY ./src/requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY ./ .

CMD [ "python", "./main.py" ]

