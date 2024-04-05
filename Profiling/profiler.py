import streamlit as st
import matplotlib.pyplot as plt
import pandas as pd
import json

st.title('Performance Comparison Tool')

# File uploaders
file1 = st.file_uploader("Drag and drop or click to upload the first JSON file", type=['json'], key="file1")
file2 = st.file_uploader("Drag and drop or click to upload the second JSON file", type=['json'], key="file2")


def calculate_fps_from_dataframe(df, function_name='DrawFrame'):
    """
    Calculates the average FPS for a specified function within a DataFrame.

    Args:
    - df: DataFrame containing profiling data, with 'Function' and 'Duration' columns.
    - function_name: The name of the function to calculate FPS for.

    Returns:
    - The average FPS for the specified function.
    """
    # Filter the DataFrame for the specified function
    function_df = df[df['Function'] == function_name]

    # Calculate the average duration in milliseconds
    avg_duration_ms = function_df['Duration'].mean() / 1000

    # Convert to seconds and calculate FPS
    if avg_duration_ms == 0:
        return float('inf')  # To avoid division by zero
    fps = 1000 / avg_duration_ms

    return fps


def compare_fps(df1, df2, function_name='DrawFrame'):
    """
    Compares the FPS of a specified function between two DataFrames.

    Args:
    - df1, df2: DataFrames containing profiling data.
    - function_name: The function name to calculate and compare FPS.

    Returns:
    - A dictionary with the FPS for each DataFrame and which had better FPS.
    """
    fps1 = calculate_fps_from_dataframe(df1, function_name)
    fps2 = calculate_fps_from_dataframe(df2, function_name)

    # Determine which DataFrame had better FPS
    better = "equal"
    if fps1 > fps2:
        better = "df1"
    elif fps2 > fps1:
        better = "df2"

    return {
        "FPS1": fps1,
        "FPS2": fps2,
        "Better Performance": better
    }

def json_to_dataframe(file):
    # Load JSON content
    data = json.load(file)

    # Prepare data for DataFrame
    flattened_data = []
    for function_name, scopes in data.items():
        if function_name == "System":  # Skip the System info
            continue
        for scope_name, instances in scopes.items():
            for instance_id, details in instances.items():
                for invocation in details.get("invocations", []):
                    flattened_data.append({
                        "Function": function_name,
                        "Scope": scope_name,
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

    better_fps = compare_fps(df1, df2)

    st.write(better_fps)

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






