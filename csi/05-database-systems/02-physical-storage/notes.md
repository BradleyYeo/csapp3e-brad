- What does the data look like?

-  How will it be accessed? All at once? In a “streaming” fashion? Random access?

- Can we generate the files in a streaming manner? If not, what data do we need beforehand (e.g. number of rows, contents of rows)?

- Can we change the files after they’ve been written, or are they immutable?

- Should the format be optimized for writes or reads? Can we support both efficiently?

- Do the files need to be “self-describing”, or can the schema be stored somewhere else?

- Should the files be divided into “chunks” or sections?

- Will the data be compressed? How? Does that have any implications for our ability to write and read the files?

- Do we need to guard against data corruption due to bit rot? How much should we do so?
