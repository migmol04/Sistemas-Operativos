#!/usr/bin/python
import argparse
import re
import apache_beam as beam
from apache_beam.io import fileio
from apache_beam.options.pipeline_options import PipelineOptions
from apache_beam.transforms import window

parser = argparse.ArgumentParser()
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument('--input-file')
group.add_argument('--input-topic')
parser.add_argument('--output-dir')

args, pipeline_args = parser.parse_known_args()
pipeline_options = PipelineOptions(pipeline_args)

with beam.Pipeline(options=pipeline_options) as p:

    if args.input_file:
        lines = p | beam.io.ReadFromText(args.input_file)
    else:
        lines = (p
            | beam.io.ReadFromPubSub(topic=args.input_topic)
            | beam.Map(lambda d: d.decode('utf-8'))
            | beam.WindowInto(window.FixedWindows(30))
        )

    counts = (lines
        | 'Split' >> beam.FlatMap(lambda x: re.findall(r'[a-z]+', x.lower()))
        | 'PairWithOne' >> beam.Map(lambda x: (x, 1))
        | 'GroupAndSum' >> beam.CombinePerKey(sum)
        | beam.Map(lambda x: x[0] + "\t" + str(x[1]))
    )

    if args.output_dir:
        counts | fileio.WriteToFiles(args.output_dir)
    else:
        counts | beam.Map(print)
