from __future__ import annotations

import yamale

def validate(src_yaml: str, schema_yaml: str) -> bool:
    """Validate a src yaml file with a schema file

    Args:
        src_yaml: path to yaml file to check
        schema_yaml: path to schema yaml file to use for check

    Returns:
        Boolean indicating if validation was successful
    """
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
        return False

    print("Validation successful")
    return True

