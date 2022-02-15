FROM python:3

LABEL Name=collector_py

WORKDIR /usr/src/app

COPY ./requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY ./src ./

CMD [ "python3", "main.py" ]

