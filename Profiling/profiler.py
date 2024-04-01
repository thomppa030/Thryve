import streamlit as st
import matplotlib.pyplot as plt
import pandas as pd
import json

st.title('Performance Comparison Tool')

# File uploaders
file1 = st.file_uploader("Drag and drop or click to upload the first JSON file", type=['json'], key="file1")
file2 = st.file_uploader("Drag and drop or click to upload the second JSON file", type=['json'], key="file2")


def json_to_dataframe(file):
    # Load JSON content
    data = json.load(file)

    # Prepare data for DataFrame
    flattened_data = []
    for function_name, function_data in data.items():
        # Skip processing for keys that don't have a dictionary value or don't contain "invocations"
        if not isinstance(function_data, dict) or not all(
                isinstance(function_data[key], dict) and "invocations" in function_data[key] for key in function_data):
            continue

        for instance_id, details in function_data.items():
            for invocation in details.get("invocations", []):
                flattened_data.append({
                    "Function": function_name,
                    "InstanceID": instance_id,
                    "Duration": invocation["duration"]
                })

    # Create DataFrame
    df = pd.DataFrame(flattened_data)

    # Calculate average duration per function
    avg_duration = df.groupby("Function")["Duration"].mean().reset_index()
    return avg_duration


def compare_dataframes(df1, df2):
    # Merge the two dataframes on the function name
    comparison_df = pd.merge(df1, df2, on="Function", suffixes=('_1', '_2'))

    # Calculate percentage difference
    comparison_df["Performance Diff (%)"] = ((comparison_df["Duration_1"] - comparison_df["Duration_2"]) /
                                             comparison_df["Duration_1"]) * 100

    # Indicator of which is better
    comparison_df["Indicator"] = comparison_df["Performance Diff (%)"].apply(lambda x: "<<" if x > 0 else ">>")

    return comparison_df


def plot_comparison_bar_chart(comparison_results):
    # Set the index to the function names for easy plotting
    comparison_results.set_index('Function', inplace=True)

    # Plotting
    fig, ax = plt.subplots()
    comparison_results['Performance Diff (%)'].plot(kind='bar', ax=ax,
                                                    color=comparison_results['Performance Diff (%)'].apply(
                                                        lambda x: 'green' if x > 0 else 'red'))

    ax.axhline(0, color='black', linewidth=0.8)
    ax.set_ylabel('Performance Difference (%)')
    ax.set_title('Performance Comparison')

    # Improve layout to handle negative and positive bars
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()

    return fig



# Display comparison
if file1 and file2:
    df1 = json_to_dataframe(file1)
    df2 = json_to_dataframe(file2)

    comparison_results = compare_dataframes(df1, df2)

    for _, row in comparison_results.iterrows():
        indicator = row["Indicator"]
        color = "green" if indicator == "<<" else "red"
        st.markdown(
            f"<p>{row['Function']}: <span style='color: {color};'>{indicator}</span> {row['Performance Diff (%)']:.2f}%</p>",
            unsafe_allow_html=True)

    # Display the chart in Streamlit
    if not comparison_results.empty:
        fig = plot_comparison_bar_chart(comparison_results)
        st.pyplot(fig)
    else:
        st.write("No comparison results to display.")
else:
    st.write("Please upload both files to proceed.")






