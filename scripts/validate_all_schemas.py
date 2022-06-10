#!/usr/bin/env python3

import os
import yamale

fsim_dir = os.path.dirname(os.path.realpath(__file__)) + "/.."

paths = {
    "sample-backup-configs/sample_config_build_recipes.yaml",
    "sample-backup-configs/sample_config_build.yaml",
    "sample-backup-configs/sample_config_hwdb.yaml",
    "sample-backup-configs/sample_config_runtime.yaml",
    "build-farm-recipes/aws_ec2.yaml",
    "build-farm-recipes/externally_provisioned.yaml",
    "run-farm-recipes/aws_ec2.yaml",
    "run-farm-recipes/externally_provisioned.yaml",
}

#<FILE TO CHECK AGAINST SCHEMA>: <SCHEMA>
paths_dict = dict([(f"{fsim_dir}/deploy/{p}", f"{fsim_dir}/deploy/schemas/{p}") for p in paths])

for src_yaml, schema_yaml in paths_dict.items():
    schema = yamale.make_schema(schema_yaml)
    data = yamale.make_data(src_yaml)
    try:
        print(f"Validating {src_yaml} with schema: {schema_yaml}")
        yamale.validate(schema, data)
    except yamale.YamaleError as e:
        print(f"Validation failed for {src_yaml} with schema: {schema_yaml}\n")
        for result in e.results:
            print("Error validating data '%s' with '%s'\n\t" % (result.data, result.schema))
            for error in result.errors:
                print('\t%s' % error)
        exit(1)
    print("Validation successful")
